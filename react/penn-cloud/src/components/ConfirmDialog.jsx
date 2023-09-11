import { AlertDialog, AlertDialogOverlay, Button, AlertDialogContent, AlertDialogHeader, AlertDialogFooter, AlertDialogBody, useDisclosure} from "@chakra-ui/react"
import { useRef } from 'react'

const ConfirmDialog = ({label, dialogText, onConfirm}) => {
	const { isOpen, onOpen, onClose } = useDisclosure()
	const cancelRef = useRef()
  
	return (
	  <>
		<Button colorScheme='red' onClick={onOpen} width={'90px'} height={'50px'}>
		  {label}
		</Button>
  
		<AlertDialog
		  isOpen={isOpen}
		  leastDestructiveRef={cancelRef}
		  onClose={onClose}
		>
		  <AlertDialogOverlay>
			<AlertDialogContent>
			  <AlertDialogHeader fontSize='lg' fontWeight='bold'>
				{dialogText}
			  </AlertDialogHeader>
  
			  <AlertDialogBody>
				Are you sure? You can't undo this action afterwards.
			  </AlertDialogBody>
  
			  <AlertDialogFooter>
				<Button ref={cancelRef} onClick={onClose}>
				  Cancel
				</Button>
				<Button colorScheme='red' onClick={() => {onClose(); onConfirm(); }} ml={3}>
				  Delete
				</Button>
			  </AlertDialogFooter>
			</AlertDialogContent>
		  </AlertDialogOverlay>
		</AlertDialog>
	  </>
	)
  }

  export default ConfirmDialog;
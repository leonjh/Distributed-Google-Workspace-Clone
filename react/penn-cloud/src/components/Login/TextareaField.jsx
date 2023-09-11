import { FormControl, FormErrorMessage, FormLabel, Textarea} from "@chakra-ui/react";
import { Input } from "@chakra-ui/input";
import { Field, useField } from "formik";

const TextField = ({ label }) => {
    return (
		<>
		<FormLabel>{label}</FormLabel>
        <Field name={label}>
			{({field, form, meta}) => {
				return (
					<Textarea
					value={field.value}
					onChange={field.onChange}
					isInvalid={meta.touched && meta.error}
					/>
				);
			}}
        </Field>
		</>
    );
};

export default TextField;